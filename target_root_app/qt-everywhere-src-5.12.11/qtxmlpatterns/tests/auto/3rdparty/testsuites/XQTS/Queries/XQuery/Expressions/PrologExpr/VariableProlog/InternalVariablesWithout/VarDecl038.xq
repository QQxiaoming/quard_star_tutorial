(:*******************************************************:)
(: Test: VarDecl038.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed Jun 29 14:28:57 2005                        :)
(: Purpose - Variable with no type definition            :)
(:*******************************************************:)

declare variable $x := xs:double("-1.7976931348623157E308") ; 
$x
