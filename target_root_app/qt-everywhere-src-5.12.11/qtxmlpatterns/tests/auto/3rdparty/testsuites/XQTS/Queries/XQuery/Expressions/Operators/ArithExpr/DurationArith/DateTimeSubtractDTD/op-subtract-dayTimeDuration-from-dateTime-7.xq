(:*******************************************************:)
(:Test: op-subtract-dayTimeDuration-from-dateTime-7      :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "subtract-dayTimeDuration-from-dateTime" operator used  :)
(:as an argument to the "fn:string" function).           :)
(:*******************************************************:)
 
fn:string(xs:dateTime("1989-07-05T10:10:10Z") - xs:dayTimeDuration("P01DT09H02M"))