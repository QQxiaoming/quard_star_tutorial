(:*******************************************************:)
(: Test: modules-11.xq                                   :)
(: Description: Evaluation of import module feature that applies the :)
(: string-length function to a value from an imported module.:)
(:********************************************************** :)

(: insert-start :)
import module namespace test1="http://www.w3.org/TestModules/test1";
declare variable $input-context external;
(: insert-end :)

fn:string-length(test1:ok())
