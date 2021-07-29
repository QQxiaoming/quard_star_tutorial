(:*******************************************************:)
(: Test: modules-8.xq                                    :)
(: Description: Evaluation of usage of same function name:)
(: from imported/importing module, but different namespaces.:)
(:********************************************************** :)

(: insert-start :)
import module namespace test1="http://www.w3.org/TestModules/test1";
declare namespace foo = "http://example.org";
declare variable $input-context external;

declare function foo:ok ()
{
   "ok"
};
(: insert-end :)

let $var := fn:concat(test1:ok(),foo:ok())
return
  $var