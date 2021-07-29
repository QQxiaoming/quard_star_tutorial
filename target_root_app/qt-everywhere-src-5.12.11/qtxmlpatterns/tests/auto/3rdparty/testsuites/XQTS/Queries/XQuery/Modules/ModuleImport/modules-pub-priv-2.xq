xquery version "1.1";

(:*******************************************************         :)
(: Test: modules-pub-priv-1.xq                                    :)
(: Description:  private functions in imported module are not visible. :)
(:**********************************************************      :)

import module namespace defs="http://www.w3.org/TestModules/module-pub-priv";

defs:f()
