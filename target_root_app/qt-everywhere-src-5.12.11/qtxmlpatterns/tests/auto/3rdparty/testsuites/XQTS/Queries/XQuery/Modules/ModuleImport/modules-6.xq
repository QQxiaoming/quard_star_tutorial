(:*******************************************************:)
(: Test: modules-6.xq                                    :)
(: Description: Evaluation of usage of variable from imported module :)
(: and usage of variable with same name in importing module, but with :)
(: different namespace prefix.                            :)
(:********************************************************** :)

(: insert-start :)
import module namespace test1="http://www.w3.org/TestModules/test1";
declare namespace foo = "http://example.org";
declare variable $foo:flag := 3;
declare variable $input-context external;
(: insert-end :)

let $var := $test1:flag + $foo:flag
return
  $var