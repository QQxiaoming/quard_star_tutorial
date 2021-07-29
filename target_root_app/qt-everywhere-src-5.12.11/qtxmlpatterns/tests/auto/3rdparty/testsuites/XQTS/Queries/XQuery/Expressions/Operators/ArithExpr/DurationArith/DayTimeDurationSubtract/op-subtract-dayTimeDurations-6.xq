(:*******************************************************:)
(:Test: op-subtract-dayTimeDurations-6                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "subtract-dayTimeDurations" operator that :)
(:is used as an argument to the fn:number function.      :)
(:*******************************************************:)
 
fn:number(xs:dayTimeDuration("P02DT06H09M") - xs:dayTimeDuration("P10DT08H01M"))