(:*******************************************************:)
(:Test: op-add-yearMonthDuration-to-date-10              :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The string value of "add-yearMonthDuration-to-date" operator used  :)
(:together with an "or" expression.                      :)
(:*******************************************************:)
 
fn:string((xs:date("1985-07-05Z") + xs:yearMonthDuration("P02Y02M"))) or fn:string((xs:date("1985-07-05Z") + xs:yearMonthDuration("P02Y02M")))