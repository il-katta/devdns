import logging
from typing import Any
from typing import Callable

import requests
from certbot.plugins import dns_common

logger = logging.getLogger(__name__)


class Authenticator(dns_common.DNSAuthenticator):
    description = 'Obtain certificates using a DNS TXT record (if you are using DevDNS for DNS).'

    def __init__(self, *args: Any, **kwargs: Any) -> None:
        super().__init__(*args, **kwargs)

    @classmethod
    def add_parser_arguments(cls, add: Callable[..., None], default_propagation_seconds: int = 60) -> None:
        super().add_parser_arguments(add, default_propagation_seconds)
        add(
            'api-server',
            default="http://localhost:8080",
            type=str,
            help='DevDNS api server url'
        )

    def more_info(self) -> str:
        return 'This plugin configures a DNS TXT record to respond to a dns-01 challenge using the devdns API.'

    def _setup_credentials(self):
        pass

    def _perform(self, domain: str, validation_name: str, validation: str) -> None:
        self._get_client().add_txt_record(validation_name, validation)

    def _cleanup(self, domain: str, validation_name: str, validation: str) -> None:
        print(domain, validation_name, validation)
        self._get_client().del_txt_record(validation_name, validation)

    def _get_client(self) -> "_DevdnsClient":
        return _DevdnsClient(self.conf('api-server'))


class _DevdnsClient:
    def __init__(self, server: str) -> None:
        self._server = server

    def add_txt_record(self, record_name: str, record_content: str) -> None:
        if (requests.get(
                self._get_url(record_name, record_content)
        ).status_code != 200):
            raise Exception("devdns api: failed request")

    def del_txt_record(self, record_name: str, record_content: str) -> None:
        requests.delete(
            self._get_url(record_name, record_content)
        )

    def _get_url(self, record_name: str, record_content: str) -> str:
        url_ = f"{self._server}/v1/request/{record_content}/{record_name}"
        return url_


