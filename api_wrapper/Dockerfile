FROM docker.io/nginx/unit:1.28.0-python3.10

ADD requirements.txt /requirements.txt

RUN set -x && \
    pip install --no-cache-dir cython && \
    pip install --no-cache-dir --no-build-isolation --no-binary :all: falcon && \
    pip install --no-cache-dir --no-binary :all: -r /requirements.txt

ADD . /app
WORKDIR /app

RUN set -x && \
    python setup.py install && \
    rm -rf /app

COPY unit.json /docker-entrypoint.d/config.json
