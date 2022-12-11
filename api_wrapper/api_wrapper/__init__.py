import falcon
import falcon.asgi
from powerdns.interface import PDNSZone

from . import settings
from .converters import *


class ApiV1RequestSplittedResource:
    async def on_get(self, req, resp, token: str, ip_address: str, name: str = None):
        resp.status = falcon.HTTP_200
        resp.content_type = falcon.MEDIA_TEXT
        resp.text = (
            f"token: {token}\n"
            f"ip_address: {ip_address}\n"
            f"name: {name}"
        )


import powerdns


class ApiV1RequestResource:
    def __init__(self):
        self._powerdns: powerdns.PDNSEndpoint = powerdns.PDNSEndpoint(
            powerdns.PDNSApiClient(api_endpoint=settings.PDNS_API_URL, api_key=settings.PDNS_API_KEY)
        )
        self._dns_zone: PDNSZone = self._powerdns.servers[0].get_zone(f"{settings.BASE_DOMAIN}.")
        if self._dns_zone is None:
            raise Exception("invalid dns zone")

    async def on_get(self, req, resp, token: str, record: str):
        self._delete_all(record, 'TXT')
        self._dns_zone.create_records([
            powerdns.RRSet(record, 'TXT', [token])
        ])
        resp.status = falcon.HTTP_200
        resp.text = "ok"

    async def on_delete(self, req, resp, token: str, record: str):
        self._dns_zone.delete_records([
            powerdns.RRSet(record, 'TXT', [token])
        ])
        resp.status = falcon.HTTP_200
        resp.text = "ok"

    def _delete_all(self, record: str, type: str):
        records = self._dns_zone.get_record(record)
        self._dns_zone.delete_records(
            [
                powerdns.RRSet(name=r['name'], rtype=r['type'], records=r['records'], ttl=r['ttl'])
                for r in records
                if r['type'] == type
            ]
        )


app = falcon.asgi.App()

app.router_options.converters['ip'] = IpConverter
app.router_options.converters['dns_name'] = DnsNameConverter
app.router_options.converters['dns_record'] = DNSRecordConverter
app.router_options.converters['token'] = TokenConverter

app.add_route('/v1/_request/{token:token}/{ip_address:ip}/{prefix:dns_name}', ApiV1RequestSplittedResource())

app.add_route(f'/v1/request/{{token:token}}/{{record:dns_record("{settings.BASE_DOMAIN}")}}', ApiV1RequestResource())
