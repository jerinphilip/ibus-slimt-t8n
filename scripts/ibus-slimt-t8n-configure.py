#!/usr/bin/env python3
import io
import os
from argparse import ArgumentParser

import slimt
import yaml
from slimt import REPOSITORY


class IBusSlimtT8nConfig:
    def __init__(self):
        self.models = []
        self.languages = set()
        pass

    def add_model(self, model):
        self.models.append(model)
        direction = model["direction"]
        self.languages.add(direction["source"])
        self.languages.add(direction["target"])

    def set_default(self, model_info):
        self.default = {"source": model_info["src"], "target": model_info["trg"]}

    def set_verify(self, verify):
        self.verify = verify

    def export(self, path):
        payload = {
            "models": self.models,
            "languages": list(self.languages),
            "default": self.default,
            "verify": self.verify,
        }

        with open(path, "w+") as fp:
            export = yaml.dump(payload, fp)


def retrieve(model_info, config_path):
    dirname = os.path.dirname(config_path)
    with open(config_path) as config_file:
        data = yaml.safe_load(config_file)
        shortlist = data.get("shortlist", None)
        return {
            "name": model_info["code"],
            "direction": {"source": model_info["src"], "target": model_info["trg"]},
            "root": dirname,
            "model": data["models"][0],
            "vocabs": {"source": data["vocabs"][0], "target": data["vocabs"][-1]},
            "shortlist": shortlist[0] if shortlist else None,
        }


if __name__ == "__main__":
    parser = ArgumentParser()
    repositories = REPOSITORY.available()
    parser.add_argument(
        "-r",
        "--repositories",
        nargs="+",
        default=repositories,
        choices=REPOSITORY.available(),
    )

    parser.add_argument("--default", type=str, required=True)
    parser.add_argument("--verify", action="store_true")

    args = parser.parse_args()
    config = IBusSlimtT8nConfig()

    for repository in args.repositories:
        models = REPOSITORY.models(repository, filter_downloaded=True)
        for model in models:
            config_path = REPOSITORY.model_config_path(repository, model)
            model_info = REPOSITORY.model(repository, model)
            field = retrieve(model_info, config_path)
            if field["shortlist"] is not None:
                config.add_model(field)

    repository, model = args.default.split("/")
    default_model_info = REPOSITORY.model(repository, model)
    config.set_default(default_model_info)
    config.set_verify(args.verify)

    home = os.getenv("HOME")
    ibus_slimt_t8n_config_path = os.path.join(home, ".config", "ibus-slimt-t8n.yml")
    config.export(ibus_slimt_t8n_config_path)
    print("Successfully wrote configuration to", ibus_slimt_t8n_config_path)
