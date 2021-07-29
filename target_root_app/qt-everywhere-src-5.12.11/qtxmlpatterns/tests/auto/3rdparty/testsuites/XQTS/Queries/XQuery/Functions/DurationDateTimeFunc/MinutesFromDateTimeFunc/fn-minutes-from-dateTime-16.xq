(:*******************************************************:)
(:Test: minutes-from-dateTime-16                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "minutes-from-dateTime" function:)
(:as part of a "numeric-equal" expression (eq operator)  :) 
(:*******************************************************:)

fn:minutes-from-dateTime(xs:dateTime("1970-01-01T10:10:00Z")) eq fn:minutes-from-dateTime(xs:dateTime("1970-01-01T10:10:00Z"))
