(:*******************************************************:)
(:Test: op-add-dayTimeDuration-to-date-9                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The string value "add-dayTimeDuration-to-date" operator used  :)
(:together with an "and" expression.                      :)
(:*******************************************************:)
 
fn:string((xs:date("1993-12-09Z") + xs:dayTimeDuration("P03DT01H04M"))) and fn:string((xs:date("1993-12-09Z") + xs:dayTimeDuration("P01DT01H03M")))