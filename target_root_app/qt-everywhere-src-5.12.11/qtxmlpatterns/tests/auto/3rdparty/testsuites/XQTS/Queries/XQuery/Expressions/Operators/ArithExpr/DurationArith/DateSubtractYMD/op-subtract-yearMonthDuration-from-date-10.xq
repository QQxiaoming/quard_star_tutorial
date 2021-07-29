(:*******************************************************:)
(:Test: op-subtract-yearMonthDuration-from-date-10       :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The string value of "subtract-yearMonthDuration-from-date" operator used:)
(:together with an "or" expression.                      :)
(:*******************************************************:)
 
fn:string((xs:date("1985-07-05Z") - xs:yearMonthDuration("P02Y02M"))) or fn:string((xs:date("1985-07-05Z") - xs:yearMonthDuration("P02Y02M")))