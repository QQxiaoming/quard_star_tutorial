(:*******************************************************:)
(:Test: op-subtract-yearMonthDuration-from-date-16       :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-yearMonthDuration-from-date" operator used  :)
(:together with the numeric-equal operator "ge".         :)
(:*******************************************************:)
 
(xs:date("1977-12-12Z") - xs:yearMonthDuration("P18Y02M")) ge  xs:date("1977-12-12Z")