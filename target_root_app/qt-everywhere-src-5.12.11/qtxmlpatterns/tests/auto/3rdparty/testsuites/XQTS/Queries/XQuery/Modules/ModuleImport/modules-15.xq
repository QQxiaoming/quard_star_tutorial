(:*******************************************************:)
(: Test: modules-15.xq                                   :)
(: Description: Evaluation of importing a library module :)
(: whose namespace uri is set to "".                     :) 
(:********************************************************** :)

(: insert-start :)
import module namespace test2="http://www.w3.org/TestModules/test2";
declare variable $input-context external;
(: insert-end :)

"aaa"
