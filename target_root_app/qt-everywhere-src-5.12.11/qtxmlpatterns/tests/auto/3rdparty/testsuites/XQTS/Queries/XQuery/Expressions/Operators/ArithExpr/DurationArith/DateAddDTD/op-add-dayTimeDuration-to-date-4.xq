(:*******************************************************:)
(:Test: op-add-dayTimeDuration-to-date-4                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The string value"add-dayTimeDuration-to-date" operator that  :)
(:return true and used together with fn:not.             :)
(:*******************************************************:)
 
fn:not(fn:string(xs:date("1998-09-12Z") + xs:dayTimeDuration("P02DT07H01M")))