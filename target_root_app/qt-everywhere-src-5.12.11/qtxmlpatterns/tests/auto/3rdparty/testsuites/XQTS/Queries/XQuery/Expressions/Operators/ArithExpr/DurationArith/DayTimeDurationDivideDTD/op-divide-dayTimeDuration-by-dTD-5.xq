(:*******************************************************:)
(:Test: op-divide-dayTimeDuration-by-dTD-5               :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                     :)
(:Purpose: Evaluates The "divide-dayTimeDuration-by-dTD" operator that  :)
(:is used as an argument to the fn:boolean function.     :)
(:*******************************************************:)
 
fn:boolean(xs:dayTimeDuration("P05DT09H08M") div xs:dayTimeDuration("P03DT08H06M"))