(:*******************************************************:)
(:Test: op-multiply-dayTimeDuration-5                    :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29 2005                                     :)
(:Purpose: Evaluates The "multiply-dayTimeDuration" function that  :)
(:is used as an argument to the fn:boolean function.     :)
(:*******************************************************:)
 
fn:boolean(fn:string(xs:dayTimeDuration("P05DT09H08M") *2.0))