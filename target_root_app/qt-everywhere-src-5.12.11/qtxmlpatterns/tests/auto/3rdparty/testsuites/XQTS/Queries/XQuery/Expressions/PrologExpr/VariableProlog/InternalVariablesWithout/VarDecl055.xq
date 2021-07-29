(:*******************************************************:)
(: Test: VarDecl055.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed Jun 29 14:28:57 2005                        :)
(: Purpose - Variable with no type definition            :)
(:*******************************************************:)

declare variable $x := (xs:date("1993-03-31") , xs:boolean("true"), xs:string("abc")) ; 
$x
