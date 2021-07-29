(:*******************************************************:)
(: Test: VarDecl062.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed Jun 29 14:28:57 2005                        :)
(: Purpose - Variable with no type definition            :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

declare variable $x := $input-context/processing-instruction() ; 
$x
