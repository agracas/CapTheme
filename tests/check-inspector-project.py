"""Verify the real project saved by inspector-smoke.cpp, including animation."""
import sys
import xml.etree.ElementTree as ET

root = ET.parse(sys.argv[1]).getroot()
transforms = []
for effect in root.iter("filter"):
    props = {p.get("name"): p.text for p in effect.findall("property")}
    if props.get("kdenlive_id") == "qtblend":
        transforms.append(props)

assert len(transforms) == 1, f"Expected one Transform filter, got {len(transforms)}"
frames = transforms[0]["rect"].split(";")
assert len(frames) == 2, frames
first_time, first_rect = frames[0].split("=", 1)
last_time, last_rect = frames[1].split("=", 1)
assert first_time != last_time
first = list(map(float, first_rect.split()))
last = list(map(float, last_rect.split()))
assert first[0] == 240 and last[0] == 480, (first, last)
assert first[2:4] == last[2:4] == [1440, 810], (first, last)
assert abs(first[4] - 0.8) < 0.0001 and abs(last[4] - 0.8) < 0.0001
print("PASS: one Transform, scale 75%, opacity 80%, and two position keyframes persisted.")
