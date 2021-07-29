(:*******************************************************:)
(: Test: modules-14.xq                                   :)
(: Description: Evaluation of import module feature that applies :)
(: the xs:integer function to a value from an imported module.:)
(:********************************************************** :)

(: insert-start :)
import module namespace test1="http://www.w3.org/TestModules/test1";
declare variable $input-context external;
(: insert-end :)

xs:integer($test1:flag)
