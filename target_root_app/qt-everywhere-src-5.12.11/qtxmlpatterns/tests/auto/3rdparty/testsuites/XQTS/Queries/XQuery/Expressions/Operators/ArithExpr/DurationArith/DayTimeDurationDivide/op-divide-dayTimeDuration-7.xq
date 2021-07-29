(:*******************************************************:)
(:Test: op-divide-dayTimeDuration-7                      :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "divide-dayTimeDuration" function used  :)
(:as an argument to the "fn:string" function).           :)
(:*******************************************************:)
 
fn:string(xs:dayTimeDuration("P03DT04H08M") div 2.0)