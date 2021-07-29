(:*******************************************************:)
(:Test: year-from-date-12                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "year-from-date" function       :)
(:as part of a "idiv" expression.                        :) 
(:*******************************************************:)

fn:year-from-date(xs:date("1970-01-01Z")) idiv fn:year-from-date(xs:date("1970-01-01Z"))
