<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://schemas.microsoft.com/wix/2006/wi"
    xmlns:wix="http://schemas.microsoft.com/wix/2006/wi">

    <xsl:template match="wix:File">
        <xsl:copy>
            <xsl:apply-templates select="@*" />
            <xsl:attribute name="ReadOnly">
                <xsl:text>yes</xsl:text>
            </xsl:attribute>
            <xsl:apply-templates select="*" />
        </xsl:copy>
    </xsl:template>

    <xsl:template match="*">
        <xsl:copy>
            <xsl:apply-templates select="@*" />
            <xsl:apply-templates select="* | text()"/>
        </xsl:copy>
    </xsl:template>

    <xsl:template match="@* | text()">
        <xsl:copy />
    </xsl:template>

    <xsl:template match="/">
        <xsl:apply-templates />
    </xsl:template>

</xsl:stylesheet>