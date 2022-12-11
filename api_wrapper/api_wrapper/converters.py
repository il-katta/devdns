import re

import falcon.routing

__all__ = [
    'IpConverter', 'DnsNameConverter', 'DNSRecordConverter', 'TokenConverter'
]


class RegExConverter(falcon.routing.converters.BaseConverter):
    def __init__(self, regex):
        self._re = re.compile(regex)

    def convert(self, value):
        if self._re.match(value):
            return value
        return None


class IpConverter(RegExConverter):
    def __init__(self):
        super().__init__(r"^((25[0-5]|(2[0-4]|1\d|[1-9]|)\d).?){3}((25[0-5]|(2[0-4]|1\d|[1-9]|)\d))$")


class DnsNameConverter(RegExConverter):
    def __init__(self):
        super().__init__(r"^((?!-))(xn--)?[a-z0-9][a-z0-9-_]{0,61}$")


class DNSRecordConverter(falcon.routing.converters.BaseConverter):
    def __init__(self, base_domain: str):
        self._base_domain = base_domain
        self._re_w_base = re.compile(
            r"^_acme-challenge\.(((?!-))(xn--)?[a-z0-9][a-z0-9-_]{0,61}[a-z0-9]{0,1}\.)?((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.){4}(xn--)?([a-z0-9\-]{1,61}|[a-z0-9-]{1,30}\.)?([a-z0-9\-]{1,61}|[a-z0-9-]{1,30}\.[a-z]{2,})$"
        )
        self._re_wo_domain = re.compile(
            r"^(((?!-))(xn--)?[a-z0-9][a-z0-9-_]{0,61}[a-z0-9]{0,1}\.)?((25[0-5]|(2[0-4]|1\d|[1-9]|)\d).?){3}((25[0-5]|(2[0-4]|1\d|[1-9]|)\d))$"
        )

    def convert(self, value):
        if value.endswith(self._base_domain):
            if self._re_w_base.match(value):
                wo_domain = value.rstrip(f".{self._base_domain}")
                return self._add_domain(wo_domain)
            else:
                return None
        else:
            if self._re_wo_domain.match(value):
                return self._add_domain(value)
            else:
                return None

    def _add_domain(self, value):
        return f"{value}.{self._base_domain}."


class TokenConverter(falcon.routing.converters.BaseConverter):
    def __init__(self):
        super().__init__()
        self._re = re.compile(r"^[a-zA-Z0-9_-]+$")

    def convert(self, value):
        if not self._re.match(value):
            raise Exception("token value not valid")
        return f'"{value}"'
