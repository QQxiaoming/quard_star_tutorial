(:*******************************************************:)
(:Test: op-divide-dayTimeDuration-by-dTD-7               :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-dayTimeDuration-by-dTD" operator used  :)
(:as an argument to the "fn:string" function.            :)
(:*******************************************************:)
 
fn:string(xs:dayTimeDuration("P08DT06H08M") div xs:dayTimeDuration("P08DT06H08M"))