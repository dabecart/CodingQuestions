from dataclasses import dataclass, asdict, fields, field
import json
from typing import List

@dataclass
class Item:
    id: int             = field(default=-1)
    name: str           = field(default="Undeclared")
    category: str       = field(default="Undetermined")
    repetitions: int    = field(default=1)
    enabled: bool       = field(default=False)
    runcode: str        = field(default="")

def saveItemsToFile(items: List[Item], filename: str) -> None:
    with open(filename, 'w') as file:
        json.dump([asdict(item) for item in items], file)

def loadItemsFromFile(filename: str) -> List[Item]:
    with open(filename, 'r') as file:
        items_dict = json.load(file)
        # Create a set of field names from the dataclass
        item_fields = {field.name for field in fields(Item)}
        items = []
        for item_dict in items_dict:
            # Filter the dictionary to only include valid fields
            filtered_dict = {k: v for k, v in item_dict.items() if k in item_fields}
            # # Add missing fields with None
            # for field in item_fields - filtered_dict.keys():
            #     filtered_dict[field] = None
            items.append(Item(**filtered_dict))
        return items
            