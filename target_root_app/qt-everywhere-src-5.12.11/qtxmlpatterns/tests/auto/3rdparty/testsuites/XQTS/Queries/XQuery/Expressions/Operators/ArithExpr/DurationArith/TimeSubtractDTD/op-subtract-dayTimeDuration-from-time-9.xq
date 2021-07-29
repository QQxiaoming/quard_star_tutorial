(:*******************************************************:)
(:Test: op-subtract-dayTimeDuration-from-time-9          :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                    :)
(:Purpose: Evaluates The string value "subtract-dayTimeDuration-from-time" subtract used :)
(:together with and "and" expression.                    :)
(:*******************************************************:)
 
fn:string((xs:time("10:10:10Z") - xs:dayTimeDuration("P02DT09H02M"))) and fn:string((xs:time("09:02:02Z") - xs:dayTimeDuration("P04DT04H04M")))