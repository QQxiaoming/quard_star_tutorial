(:*******************************************************:)
(: Test: combined-errors-1.xq                                    :)
(: Description: Evaluates simple module import to generate error code if feature not supported. :)
(:********************************************************** :)

(: insert-start :)
import module namespace defs="http://www.w3.org/TestModules/defs";
declare variable $input-context1 external;
(: insert-end :)

"ABC"