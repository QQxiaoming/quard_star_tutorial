(:*******************************************************:)
(:Test: op-divide-dayTimeDuration-6                      :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "divide-dayTimeDuration" operator that :)
(:is used as an argument to the fn:number function.      :)
(:*******************************************************:)
 
fn:number(xs:dayTimeDuration("P02DT06H09M") div 2.0)