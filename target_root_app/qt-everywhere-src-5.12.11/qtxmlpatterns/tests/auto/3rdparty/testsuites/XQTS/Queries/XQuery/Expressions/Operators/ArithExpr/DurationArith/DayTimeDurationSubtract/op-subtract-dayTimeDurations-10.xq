(:*******************************************************:)
(:Test: op-subtract-dayTimeDurations-10                  :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "subtract-dayTimeDurations" function used  :)
(:together with an "or" expression.                      :)
(:*******************************************************:)
 
fn:string((xs:dayTimeDuration("P05DT09H02M") - xs:dayTimeDuration("P03DT01H04M"))) or fn:string((xs:dayTimeDuration("P05DT05H03M") - xs:dayTimeDuration("P01DT01H03M")))