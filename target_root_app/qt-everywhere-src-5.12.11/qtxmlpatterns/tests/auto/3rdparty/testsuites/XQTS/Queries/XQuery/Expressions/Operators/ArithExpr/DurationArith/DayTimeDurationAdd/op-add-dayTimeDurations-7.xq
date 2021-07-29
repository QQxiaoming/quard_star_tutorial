(:*******************************************************:)
(:Test: op-add-dayTimeDurations-7                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "add-dayTimeDurations" function used  :)
(:as an argument to the "fn:string" function).           :)
(:*******************************************************:)
 
fn:string(xs:dayTimeDuration("P03DT04H08M") + xs:dayTimeDuration("P01DT09H02M"))