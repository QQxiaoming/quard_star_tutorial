(:*******************************************************:)
(:Test: op-subtract-dayTimeDuration-from-time-7          :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "subtract-dayTimeDuration-from-time" operator used  :)
(:as an argument to the "fn:string" function).           :)
(:*******************************************************:)
 
fn:string(xs:time("19:45:55Z") - xs:dayTimeDuration("P01DT09H02M"))