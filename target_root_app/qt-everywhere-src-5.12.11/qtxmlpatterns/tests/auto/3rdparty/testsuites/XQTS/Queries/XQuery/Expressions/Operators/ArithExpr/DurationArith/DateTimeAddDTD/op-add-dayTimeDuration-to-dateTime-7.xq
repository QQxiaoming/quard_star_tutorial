(:*******************************************************:)
(:Test: op-add-dayTimeDuration-to-dateTime-7             :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "add-dayTimeDuration-to-dateTime" operator used  :)
(:as an argument to the "fn:string" function).           :)
(:*******************************************************:)
 
fn:string(xs:dateTime("1989-07-05T10:10:10Z") + xs:dayTimeDuration("P01DT09H02M"))