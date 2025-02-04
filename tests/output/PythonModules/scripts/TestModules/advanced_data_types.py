from osgeo import ogr
from pydynamind import *

class AdvancedDataTypes(Module):
        def __init__(self):
            Module.__init__(self)
            self.setIsGDALModule(True)
            self.createParameter("elements", INT, "Number of elements")
            self.elements = 100000

            self.createParameter("append", BOOL, "true if append")
            self.append = False
            self.__container = ViewContainer()

        def init(self):
            if self.append:
                self.__container = ViewContainer("component", NODE, MODIFY)
            else:
                self.__container = ViewContainer("component", NODE, WRITE)
            self.__container.addAttribute("vector", DM.Attribute.DOUBLEVECTOR, WRITE)
            self.__container.addAttribute("integer",DM.Attribute.INT, WRITE)

            views = []
            views.append(self.__container)
            self.registerViewContainers(views)
            #self.features = []
        def run(self):
            for i in range(self.elements):
                f = self.__container.create_feature()
                a = []
                for i in range(1000):
                    a.append(i*1.0)
                dm_set_double_list(f, "vector", a)
                f.SetField("integer", i)
                if i % 100000 == 0:
                    self.__container.sync()
            self.__container.finalise()