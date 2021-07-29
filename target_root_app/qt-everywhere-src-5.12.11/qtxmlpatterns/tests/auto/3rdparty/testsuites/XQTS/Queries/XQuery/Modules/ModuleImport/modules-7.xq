(:*******************************************************:)
(: Test: modules-7.xq                                    :)
(: Description: Evaluation of of an importing module that uses :)
(: both a variable and a function from an importing module. :)
(:********************************************************** :)

(: insert-start :)
import module namespace test1="http://www.w3.org/TestModules/test1";
declare variable $input-context external;
(: insert-end :)

let $var := fn:concat(xs:string($test1:flag),xs:string(test1:ok()))
return
  $var