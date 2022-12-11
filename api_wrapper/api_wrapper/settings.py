import os
BASE_DOMAIN = os.getenv("BASE_DOMAIN", "devdns.sh").rstrip(".")
PDNS_API_URL = os.getenv("PDNS_API_URL", "http://localhost:8081/api/v1")
PDNS_API_KEY = os.getenv("PDNS_API_KEY", None)
