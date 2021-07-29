(:*******************************************************:)
(:Test: op-subtract-dayTimeDuration-from-dateTime-4      :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The string value of "subtract-dayTimeDuration-from-dateTime" :)
(:operator that return true and used together with fn:not.:)
(:*******************************************************:)
 
fn:not(fn:string(xs:dateTime("1998-09-12T13:23:23Z") - xs:dayTimeDuration("P02DT07H01M")))