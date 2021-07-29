(:*******************************************************:)
(: Test: modules-16.xq                                   :)
(: Description: Evaluation of module import with "xml" prefix. :)
(:********************************************************** :)

(: insert-start :)
import module namespace xml="http://www.w3.org/TestModules/test1";
declare variable $input-context external;
(: insert-end :)

xs:string($xml:flag)