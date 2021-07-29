(:*******************************************************:)
(:Test: op-divide-dayTimeDuration-by-dTD-6               :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-dayTimeDuration-by-dTD" operator that :)
(:is used as an argument to the fn:number function.      :)
(:*******************************************************:)
 
fn:number(xs:dayTimeDuration("P02DT06H09M") div xs:dayTimeDuration("P02DT06H09M"))