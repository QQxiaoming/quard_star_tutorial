xquery version "1.1";

(:*******************************************************         :)
(: Test: modules-pub-priv-1.xq                                    :)
(: Description:  public functions in imported module are visible. :)
(:**********************************************************      :)

import module namespace defs="http://www.w3.org/TestModules/module-pub-priv";

<a>{defs:g(42)}</a>
