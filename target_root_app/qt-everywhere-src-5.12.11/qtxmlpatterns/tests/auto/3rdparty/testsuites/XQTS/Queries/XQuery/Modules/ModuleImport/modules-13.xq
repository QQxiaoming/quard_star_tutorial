(:*******************************************************:)
(: Test: modules-13.xq                                   :)
(: Description: Evaluation of import module feature that performs :)
(: a subtraction operation to a value from an imported module.:)
(:********************************************************** :)

(: insert-start :)
import module namespace test1="http://www.w3.org/TestModules/test1";
declare variable $input-context external;
(: insert-end :)

$test1:flag - $test1:flag
