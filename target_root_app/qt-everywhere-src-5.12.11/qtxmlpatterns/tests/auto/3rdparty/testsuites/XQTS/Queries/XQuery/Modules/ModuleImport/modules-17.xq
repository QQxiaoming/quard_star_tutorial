(:*******************************************************:)
(: Test: modules-17.xq                                   :)
(: Description: Evaluation of module import with variables bound to a :)
(: namespace URI different from the target namespace of the module. :)
(:********************************************************** :)

(: insert-start :)
import module namespace defs="http://www.w3.org/TestModules/diffns";
declare variable $input-context external;
(: insert-end :)

"abc"