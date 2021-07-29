(:*******************************************************:)
(:Test: op-subtract-dayTimeDurations-9                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "subtract-dayTimeDurations" function used :)
(:together with and "and" expression.                    :)
(:*******************************************************:)
 
fn:string((xs:dayTimeDuration("P01DT02H01M") - xs:dayTimeDuration("P02DT09H02M"))) and fn:string((xs:dayTimeDuration("P02DT03H03M") - xs:dayTimeDuration("P04DT04H04M")))