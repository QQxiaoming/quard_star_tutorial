(:*******************************************************:)
(: Test: VarDecl044.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed Jun 29 14:28:57 2005                        :)
(: Purpose - Variable with no type definition            :)
(:*******************************************************:)

declare variable $x := xs:float("-3.4028235E38") ; 
$x
