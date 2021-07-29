(:*******************************************************:)
(:Test: fn-current-date-21                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 5, 2005                                 :)
(:Purpose: Evaluates The "fn-current-date" function used :)
(:as part of a "-" operation and a yearMonthDuration.    :)
(:*******************************************************:)

fn:current-date() - xs:yearMonthDuration("P1Y2M")
