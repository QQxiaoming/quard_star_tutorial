(:*******************************************************:)
(:Test: op-subtract-dayTimeDurations-3                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "subtract-dayTimeDurations" function as :)
(:part of a boolean expression (or operator) and the "fn:boolean" function. :)
(:*******************************************************:)
 
fn:string(xs:dayTimeDuration("P20DT20H10M") - xs:dayTimeDuration("P19DT13H10M")) or fn:false()