(:*******************************************************:)
(:Test: op-subtract-dayTimeDuration-from-time-10         :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The string value of "subtract-dayTimeDuration-from-time" operator used  :)
(:together with an "or" expression.                      :)
(:*******************************************************:)
 
fn:string((xs:time("23:45:50Z") - xs:dayTimeDuration("P03DT01H04M"))) or fn:string((xs:time("23:45:50Z") + xs:dayTimeDuration("P01DT01H03M")))