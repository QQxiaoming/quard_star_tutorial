(:*******************************************************:)
(: Test: LetExpr016.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed May 11 13:55:57 2005                        :)
(: Purpose : Use a sequence of nodes :)
(:*******************************************************:)
let $a := (<elem1/>, <elem2/>, <elem3 att="test"/>)
return <root>{$a}</root>
