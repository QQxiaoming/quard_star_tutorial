(:*******************************************************:)
(: Test: modules-9.xq                                    :)
(: Description: Evaluation of import module feature that applies the :)
(: upper case function to a value from an imported module.:)
(:********************************************************** :)

(: insert-start :)
import module namespace test1="http://www.w3.org/TestModules/test1";
declare variable $input-context external;
(: insert-end :)

fn:upper-case(test1:ok())
