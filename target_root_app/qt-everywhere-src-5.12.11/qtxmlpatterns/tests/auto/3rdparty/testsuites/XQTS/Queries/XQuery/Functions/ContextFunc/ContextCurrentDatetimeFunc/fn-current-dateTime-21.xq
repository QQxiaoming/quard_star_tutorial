(:*******************************************************:)
(:Test: fn-current-dateTime-21                           :)
(:Written By: Carmelo Montanez                           :)
(:dateTime: December 5, 2005                             :)
(:Purpose: Evaluates The "fn-current-dateTime" function used :)
(:as part of a "-" operation and a yearMonthDuration.    :)
(:*******************************************************:)

fn:current-dateTime() - xs:yearMonthDuration("P1Y2M")
