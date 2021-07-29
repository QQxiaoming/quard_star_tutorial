(:*******************************************************:)
(: Test: modules-3.xq                                    :)
(: Description: Evaluates that module importing is not transitive. :)
(:********************************************************** :)

(: insert-start :)
import module namespace defs="http://www.w3.org/TestModules/defs";
declare variable $input-context external;
(: insert-end :)

let $var := $test1:flag + 1
return
  $var