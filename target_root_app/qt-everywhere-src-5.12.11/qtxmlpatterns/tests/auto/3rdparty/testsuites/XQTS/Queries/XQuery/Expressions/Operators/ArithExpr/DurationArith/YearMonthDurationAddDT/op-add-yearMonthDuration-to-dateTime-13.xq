(:*******************************************************:)
(:Test: op-add-yearMonthDuration-to-dateTime-13          :)
(:Written By: Carmelo Montanez                           :)
(:date: July 5, 2005                                     :)
(:Purpose: Evaluates The "add-yearMonthDuration-to-dateTime" operator used  :)
(:together with the numeric-equal-operator "eq".         :)
(:*******************************************************:)
 
(xs:dateTime("1980-05-05T05:05:05Z") + xs:yearMonthDuration("P23Y11M")) eq xs:dateTime("1980-05-05T05:05:05Z")