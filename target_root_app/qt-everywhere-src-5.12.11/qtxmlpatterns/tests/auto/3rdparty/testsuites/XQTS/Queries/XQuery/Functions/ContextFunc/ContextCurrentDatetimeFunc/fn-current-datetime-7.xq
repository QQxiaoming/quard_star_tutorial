(:*******************************************************:)
(:Test: fn-current-dateTime-7                            :)
(:Written By: Carmelo Montanez                           :)
(:dateTime: December 5, 2005                             :)
(:Purpose: Evaluates The "fn-current-dateTime" function used :)
(:as part of a "-" expression and a dayTimeDuration.     :)
(:*******************************************************:)

fn:current-dateTime() - xs:dayTimeDuration("P3DT1H15M")
