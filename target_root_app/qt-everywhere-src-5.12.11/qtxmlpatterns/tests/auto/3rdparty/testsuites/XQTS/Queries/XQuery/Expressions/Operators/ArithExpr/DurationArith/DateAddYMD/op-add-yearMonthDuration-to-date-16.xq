(:*******************************************************:)
(:Test: op-add-yearMonthDuration-to-date-16              :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "add-yearMonthDuration-to-date" operator used  :)
(:together with the numeric-equal operator "ge".         :)
(:*******************************************************:)
 
(xs:date("1977-12-12Z") + xs:yearMonthDuration("P18Y02M")) ge  xs:date("1977-12-12Z")