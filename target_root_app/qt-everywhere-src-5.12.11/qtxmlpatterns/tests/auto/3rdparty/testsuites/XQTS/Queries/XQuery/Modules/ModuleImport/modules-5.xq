(:*******************************************************:)
(: Test: modules-5.xq                                    :)
(: Description: Module that uses a variable from an imported module :)
(: and modifies a variable.  The importing module in turn uses that modified :)
(: value. :)
(:********************************************************** :)

(: insert-start :)
import module namespace defs="http://www.w3.org/TestModules/defs";
declare variable $input-context external;
(: insert-end :)

let $var := $defs:var2 + 1
return
  $var