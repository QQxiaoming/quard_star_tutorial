(:*******************************************************:)
(: Test: modules-18.xq                                   :)
(: Description: Evaluation of module import with two imports, :)
(: one of which import the other and uses its function. :)
(:********************************************************** :)

(: insert-start :)
import module namespace mod1="http://www.w3.org/TestModules/module1";
import module namespace mod2="http://www.w3.org/TestModules/module2";
declare variable $input-context external;
(: insert-end :)

mod1:x(),mod2:y()