(:*******************************************************:)
(:Test: op-add-dayTimeDuration-to-time-7                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "add-dayTimeDuration-to-time" function used  :)
(:as an argument to the "fn:string" function).           :)
(:*******************************************************:)
 
fn:string(xs:time("10:02:03Z") + xs:dayTimeDuration("P01DT09H02M"))