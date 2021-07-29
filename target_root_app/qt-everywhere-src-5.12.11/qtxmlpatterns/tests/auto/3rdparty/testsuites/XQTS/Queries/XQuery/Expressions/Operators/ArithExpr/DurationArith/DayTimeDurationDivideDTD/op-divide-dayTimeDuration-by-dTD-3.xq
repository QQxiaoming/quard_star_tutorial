(:*******************************************************:)
(:Test: op-divide-dayTimeDuration-by-dTD-3               :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-dayTimeDuration-by-dTD" operator as :)
(:part of a boolean expression (or operator) and the "fn:boolean" function. :)
(:*******************************************************:)
 
xs:dayTimeDuration("P20DT20H10M") div xs:dayTimeDuration("P19DT13H10M") or fn:false()