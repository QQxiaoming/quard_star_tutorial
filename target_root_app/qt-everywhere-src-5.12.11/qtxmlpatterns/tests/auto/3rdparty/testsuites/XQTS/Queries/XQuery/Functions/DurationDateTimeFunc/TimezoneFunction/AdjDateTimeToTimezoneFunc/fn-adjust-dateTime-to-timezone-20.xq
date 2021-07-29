(:*******************************************************:)
(:Test: adjust-dateTime-to-timezone-20                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: August 10, 2005                                  :)
(:Test Description: Evaluates The "adjust-dateTime-to-timezone" function   :)
(:as part of a comparisson expression (ge operator).     :)
(:*******************************************************:)

fn:adjust-dateTime-to-timezone(xs:dateTime("2002-03-07T10:00:00-04:00")) ge fn:adjust-dateTime-to-timezone(xs:dateTime("2005-03-07T10:00:00-04:00"))