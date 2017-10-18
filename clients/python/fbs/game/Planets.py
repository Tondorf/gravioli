# automatically generated by the FlatBuffers compiler, do not modify

# namespace: game

import flatbuffers

class Planets(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsPlanets(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = Planets()
        x.Init(buf, n + offset)
        return x

    # Planets
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # Planets
    def Planets(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from .Planet import Planet
            obj = Planet()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # Planets
    def PlanetsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

def PlanetsStart(builder): builder.StartObject(1)
def PlanetsAddPlanets(builder, planets): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(planets), 0)
def PlanetsStartPlanetsVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def PlanetsEnd(builder): return builder.EndObject()