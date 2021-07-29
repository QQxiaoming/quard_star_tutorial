(:*******************************************************:)
(:Test: op-subtract-dayTimeDuration-from-date-9          :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The string value of "subtract-dayTimeDuration-from-date" operator used  :)
(:together with an "and" expression.                      :)
(:*******************************************************:)
 
fn:string((xs:date("1993-12-09Z") - xs:dayTimeDuration("P03DT01H04M"))) and fn:string((xs:date("1993-12-09Z") - xs:dayTimeDuration("P01DT01H03M")))